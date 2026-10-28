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
#include "jit/jit.h"
#include "rt/model.h"
#include "tool.h"

jit_t *tool_new_jit(unit_registry_t *ur, mir_context_t *mc)
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
