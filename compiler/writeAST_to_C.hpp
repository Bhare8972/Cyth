
/*
Copyright 2019 Brian Hare

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

This file defines the functions to write the AST to C. Assuming the AST has been fully processed.
*/

#ifndef WRITEAST_TO_C_191101195347
#define WRITEAST_TO_C_191101195347

#include <string>
#include "cyth_AST.hpp"

void write_module_to_C(module_AST_ptr module, std::string fout_name);

#endif // WRITEAST_TO_C_191101195347

