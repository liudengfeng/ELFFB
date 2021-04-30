/**
* Copyright (c) 2017-present, Facebook, Inc.
* All rights reserved.

* This source code is licensed under the BSD-style license found in the
* LICENSE file in the root directory of this source tree.
*/

#include <pybind11/pybind11.h>
#include <pybind11/stl_bind.h>
#include <pybind11/stl.h>

#include <string>
#include <iostream>

#include "engine/wrapper_template.h"
#include "wrapper_callback.h"
#include "ai.h"


class GameContext {
public:
    using GC = Context;
    using Wrapper = WrapperT<WrapperCallbacks, GC::Comm, PythonOptions>;

private:
    int _T;
    std::unique_ptr<GC> _context;
    Wrapper _wrapper;

public:
    GameContext(const ContextOptions& context_options, const PythonOptions& options) {
      _T = context_options.T;
      GameDef::GlobalInit();

      _context.reset(new GC{context_options, options});
    }

    void Start() {
        _context->Start(
            [&](int game_idx, const ContextOptions &context_options, const PythonOptions &options, const std::atomic_bool &done, Comm *comm) {
                    _wrapper.thread_main(game_idx, context_options, options, done, comm);
            });    }

    const std::string &game_unittype2str(int unit_type) const {
        return _UnitType2string((UnitType)unit_type);
    }
    const std::string &game_aistate2str(int ai_state) const {
        return _AIState2string((AIState)ai_state);
    }

    int get_num_actions() const { return GameDef::GetNumAction(); }
    int get_num_unittype() const { return GameDef::GetNumUnitType(); }

    std::map<std::string, int> GetParams() const {
        return std::map<std::string, int>{
            { "num_action", GameDef::GetNumAction() },
            { "num_unit_type", GameDef::GetNumUnitType() }
        };
    }

    CONTEXT_CALLS(GC, _context);

    void Stop() {
      _context.reset(nullptr); // first stop the threads, then destroy the games
    }

    EntryInfo EntryFunc(const std::string &key) {
        auto *mm = GameState::get_mm(key);
        if (mm == nullptr) return EntryInfo();

        std::string type_name = mm->type();

        if (key == "s") return EntryInfo(key, type_name, {2, 20, 20});
        else if (key == "last_r" || key == "terminal" || key == "last_terminal" || key == "base_hp_level" || key == "id" || key == "seq" || key == "game_counter") return EntryInfo(key, type_name);
        else if (key == "pi") return EntryInfo(key, type_name, {GameDef::GetNumAction()});
        else if (key == "a" || key == "rv" || key == "V") return EntryInfo(key, type_name);

        return EntryInfo();
    }
};

#define CONST(v) m.attr(#v) = py::int_(v)

PYBIND11_MODULE(minirts, m) {
  register_common_func<GameContext>(m);
  CONTEXT_REGISTER(GameContext)
    .def("GetParams", &GameContext::GetParams);

  // Also register other objects.
  PYCLASS_WITH_FIELDS(m, AIOptions)
    .def(py::init<>());

  PYCLASS_WITH_FIELDS(m, PythonOptions)
    .def(py::init<>())
    .def("Print", &PythonOptions::Print)
    .def("AddAIOptions", &PythonOptions::AddAIOptions);
}
