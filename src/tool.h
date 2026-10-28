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

#ifndef _TOOL_H
#define _TOOL_H

#include "prim.h"
#include "lib.h"
#include "phase.h"

typedef struct {
   lib_t                 work;        // Library with the design or NULL
   const elab_generic_t *generics;    // Top-level generic overrides
   int                   ngenerics;
   cover_data_t         *cover;       // Coverage database or NULL
   const unit_meta_t    *meta;        // Saved with the design or NULL
   bool                  no_save;     // Do not save the elaborated design
} tool_elab_options_t;

typedef struct {
   const char  *plugins;     // VHPI plugins to load, comma separated
   int          nplusargs;   // Plusargs passed to the plugins
   char       **plusargs;
} tool_options_t;

typedef struct {
   const char  *plugins;     // Plugins loaded after VHPI is initialised
   int          nplusargs;   // Plusargs for this simulation only
   char       **plusargs;
   bool         vhpi;        // Initialise VHPI even without plugins
} tool_open_options_t;

tool_t *tool_new(const tool_options_t *opts);
void tool_free(tool_t *tool);
void tool_set_top(tool_t *tool, tree_t top, cover_data_t *cover);
tree_t tool_elaborate(tool_t *tool, const tool_elab_options_t *opts, int ntops,
                      const ident_t *tops);
rt_model_t *tool_open(tool_t *tool, const tool_open_options_t *opts);
void tool_close(tool_t *tool);
rt_model_t *tool_model(tool_t *tool);
tree_t tool_top(tool_t *tool);
jit_t *tool_jit(tool_t *tool);
void tool_interrupt(tool_t *tool);

#endif  // _TOOL_H
