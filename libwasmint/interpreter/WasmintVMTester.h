/*
 * Copyright 2015 WebAssembly Community Group
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


#ifndef WASMINT_WASMINTVMTESTER_H
#define WASMINT_WASMINTVMTESTER_H

#include "WasmintVM.h"

namespace wasmint {

    ExceptionMessage(FailedVMTest)
    ExceptionMessage(CantTestRunningVM)

    class WasmintVMTester {

        WasmintVM& vm_;

        std::size_t maxIntermediateStates = 10;
        std::size_t intermediateStateInterval = 50;
        std::map<InstructionCounter, VMState> selectedIntermediateStates;

    public:
        WasmintVMTester(WasmintVM& vm) : vm_(vm) {
        }

        void stepUntilFinished();

    };
}



#endif //WASMINT_WASMINTVMTESTER_H
