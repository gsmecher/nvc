//
//  Copyright (C) 2026  Nick Gasson
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.
//

#include "util.h"
#include "common.h"
#include "diag.h"
#include "jit/jit.h"
#include "lib.h"
#include "lower.h"
#include "mir/mir-unit.h"
#include "phase.h"
#include "rt/assert.h"
#include "rt/model.h"
#include "tool.h"
#include "vhpi/vhpi-model.h"
#include "vpi/vpi-model.h"

#include <assert.h>
#include <stdlib.h>

typedef struct _tool {
   char            *plugins;
   int              nplusargs;
   char           **plusargs;
   bool             loaded;
   vhpi_context_t  *vhpi;
   vpi_context_t   *vpi;
   tree_t           top;
   cover_data_t    *cover;
   mir_context_t   *mir;
   unit_registry_t *registry;
   jit_t           *jit;
   rt_model_t      *model;
   bool             open;
} tool_t;

static char **copy_plusargs(int nplusargs, char **plusargs)
{
   char **copy = xmalloc_array(nplusargs, sizeof(char *));
   for (int i = 0; i < nplusargs; i++)
      copy[i] = xstrdup(plusargs[i]);

   return copy;
}

static void free_plusargs(int nplusargs, char **plusargs)
{
   for (int i = 0; i < nplusargs; i++)
      free(plusargs[i]);

   free(plusargs);
}

static void tool_load(tool_t *tool)
{
   // Plugins are loaded into each new VHPI context when it is first used
   if (tool->loaded || tool->plugins == NULL)
      return;

   if (tool->nplusargs > 0)
      vhpi_set_plusargs(tool->vhpi, tool->nplusargs, tool->plusargs);

   vhpi_load_plugins(tool->plugins);
   tool->loaded = true;
}

tool_t *tool_new(const tool_options_t *opts)
{
   tool_t *tool = xcalloc(sizeof(tool_t));
   tool->vhpi = vhpi_context_new();
   tool->vpi  = vpi_context_new();

   if (opts != NULL && opts->plugins != NULL) {
      tool->plugins   = xstrdup(opts->plugins);
      tool->nplusargs = opts->nplusargs;
      tool->plusargs  = copy_plusargs(opts->nplusargs, opts->plusargs);

      tool_load(tool);
   }

   return tool;
}

static void tool_free_model(tool_t *tool)
{
   if (tool->model != NULL)
      model_free(tool->model);

   if (tool->jit != NULL)
      jit_free(tool->jit);

   if (tool->registry != NULL)
      unit_registry_free(tool->registry);

   if (tool->mir != NULL)
      mir_context_free(tool->mir);

   tool->model    = NULL;
   tool->jit      = NULL;
   tool->registry = NULL;
   tool->mir      = NULL;
}

void tool_free(tool_t *tool)
{
   if (tool->open)
      model_end(tool->model);

   vhpi_context_free(tool->vhpi);
   vpi_context_free(tool->vpi);

   tool_free_model(tool);

   free_plusargs(tool->nplusargs, tool->plusargs);

   free(tool->plugins);
   free(tool);
}

void tool_set_top(tool_t *tool, tree_t top, cover_data_t *cover)
{
   if (tool->top == top)
      return;

   // Any model elaborated from the previous design is no longer valid
   tool_close(tool);
   tool_free_model(tool);

   tool->top   = top;
   tool->cover = cover;
}

static jit_t *tool_new_jit(unit_registry_t *ur, mir_context_t *mc)
{
   jit_t *jit = jit_new(ur, mc);

#if defined HAVE_LLVM && 1
   jit_register_llvm_plugin(jit);
#elif defined ARCH_X86_64 && 0
   jit_register_native_plugin(jit);
#endif

   // The runtime packages are only reachable through the JIT so these
   // calls are needed to force them to be linked
   _std_standard_init();
   _std_env_init();
   _std_reflection_init();
   _file_io_init();
   _nvc_sim_pkg_init();

   return jit;
}

tree_t tool_elaborate(tool_t *tool, const tool_elab_options_t *opts, int ntops,
                      const ident_t *tops)
{
   lib_t work = opts->work ?: lib_work();

   // Each elaboration starts from no errors as for a separate command
   reset_error_count();

   object_t **objs LOCAL = xmalloc_array(ntops, sizeof(object_t *));
   for (int i = 0; i < ntops; i++) {
      if ((objs[i] = lib_get_generic(work, tops[i], NULL)) == NULL) {
         errorf("cannot find unit %pI in library %pI", tops[i],
                lib_name(work));
         return NULL;
      }
   }

   // Elaboration starts a new simulation
   tool_set_top(tool, NULL, NULL);
   tool_load(tool);

   tool->cover    = opts->cover;
   tool->mir      = mir_context_new();
   tool->registry = unit_registry_new(tool->mir);
   tool->jit      = tool_new_jit(tool->registry, tool->mir);
   tool->model    = model_new(tool->jit, tool->cover);

   vhpi_run_callbacks(vhpiCbStartOfElaboration);

   tree_t top = elab(objs, ntops, opts->generics, opts->ngenerics, tool->jit,
                     tool->registry, tool->mir, tool->cover, NULL, tool->model);
   if (top == NULL) {
      tool_free_model(tool);
      return NULL;
   }

   const unit_meta_t no_meta = {};
   lib_put_meta(work, top, opts->meta ?: &no_meta);

   progress("elaborating design");

   vhpi_run_callbacks(vhpiCbEndOfElaboration);

   if (error_count() > 0) {
      tool_free_model(tool);
      return NULL;
   }

   if (!opts->no_save) {
      lib_save(work);
      progress("saving library");
   }

   tool->top = top;
   return top;
}

rt_model_t *tool_open(tool_t *tool, const tool_open_options_t *opts)
{
   if (tool->open)
      return tool->model;
   else if (tool->top == NULL)
      return NULL;

   static const tool_open_options_t no_options = {};
   if (opts == NULL)
      opts = &no_options;

   tool_load(tool);

   if (tool->model == NULL) {
      // Units cannot be lowered twice into the same registry so the
      // model must be reheated in a fresh context
      tool->mir      = mir_context_new();
      tool->registry = unit_registry_new(tool->mir);
      tool->jit      = tool_new_jit(tool->registry, tool->mir);
      tool->model    = model_new(tool->jit, tool->cover);

      reheat(tool->top, tool->registry, tool->mir, tool->cover, tool->model);
   }

   if (opts->nplusargs > 0)
      vhpi_set_plusargs(tool->vhpi, opts->nplusargs, opts->plusargs);

   // VHPI adds overhead to every cycle so is only initialised if needed
   if (opts->vhpi || tool->plugins != NULL || opts->plugins != NULL) {
      vhpi_context_initialise(tool->vhpi, tool->top, tool->model, tool->jit);
      vpi_context_initialise(tool->vpi, tool->top, tool->model, tool->jit,
                             opts->nplusargs, opts->plusargs);
   }

   if (opts->plugins != NULL)
      vhpi_load_plugins(opts->plugins);

   model_reset(tool->model);

   tool->open = true;
   return tool->model;
}

void tool_close(tool_t *tool)
{
   if (!tool->open)
      return;

   model_end(tool->model);

   // The VHPI and VPI contexts refer to the model so cannot be reused
   vhpi_context_free(tool->vhpi);
   vpi_context_free(tool->vpi);

   tool_free_model(tool);

   // Assertion counts and enables may have been changed by the design
   clear_vhdl_assert();
   for (vhdl_severity_t i = SEVERITY_NOTE; i <= SEVERITY_FAILURE; i++)
      set_vhdl_assert_enable(i, true);

   tool->vhpi   = vhpi_context_new();
   tool->vpi    = vpi_context_new();
   tool->loaded = false;

   tool->open = false;
}

rt_model_t *tool_model(tool_t *tool)
{
   return tool->open ? tool->model : NULL;
}

tree_t tool_top(tool_t *tool)
{
   return tool->top;
}

jit_t *tool_jit(tool_t *tool)
{
   return tool->open ? tool->jit : NULL;
}

void tool_interrupt(tool_t *tool)
{
   if (tool->model != NULL)
      model_interrupt(tool->model);
}
