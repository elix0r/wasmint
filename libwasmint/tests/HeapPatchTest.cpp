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


#include <cstdint>
#include <interpreter/Heap.h>
#include <cassert>

using namespace wasmint;

int main() {
    Heap heap;
    Heap heapBackup;
    HeapPatch patch;

    // create initial state we want to modify and revert back to
    heap.grow(5);
    heap.setBytes(0, {1, 2, 3, 4, 5});

    // backup our original state
    heapBackup = heap;

    // test reverting grow
    heap.grow(10, patch);
    assert(heap.size() == 15);
    assert(heap != heapBackup);
    heap.applyPatch(patch);
    assert(heap == heapBackup);

    // test reverting shrink
    heap.shrink(2, patch);
    assert(heap.size() == 3);
    assert(heap != heapBackup);
    heap.applyPatch(patch);
    assert(heap == heapBackup);

    // test set bytes
    heap.setBytes(1, {8, 9}, patch);
    assert(heap != heapBackup);
    heap.applyPatch(patch);
    assert(heap == heapBackup);
}