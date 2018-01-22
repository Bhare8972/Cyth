/*
Copyright 2015 Brian Hare

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

This is the entry point for the Cyth compiler
*/

#include "module_manager.hpp"

int main(int argc, char *argv[])
{
    module_manager cyth_module_manager(false);

    if(argc == 2)
    {
        std::cout<<"opening: "<<argv[1]<<std::endl;
        cyth_module_manager.parse_module(argv[1], true);
    }


    std::cout<<"DONE!"<<std::endl;
}
