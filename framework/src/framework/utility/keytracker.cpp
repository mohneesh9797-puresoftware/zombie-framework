#include <framework/utility/keytracker.hpp>

namespace zfw {

KeyTracker::KeyTracker(IVarSystem* varSystem) {
}

void KeyTracker::Subscribe(int id, const char* bindingName, Vkey_t defaultKey) {
    if (id >= trackedKeys.size()) {
        trackedKeys.resize(id + 1);
        isHeld.resize(id + 1);
    }

    // TODO: override from variable by bindingName
    trackedKeys[id] = defaultKey;
    isHeld[id] = false;
}

void KeyTracker::OnFrame() {
    // wasPressed.clear();
}

void KeyTracker::Handle(EventVkey const& ev) {
    for (size_t i = 0; i < trackedKeys.size(); i++) {
        if (Vkey::Test(ev.input, trackedKeys[i])) {
            isHeld[i] = (ev.input.flags & VKEY_PRESSED) == VKEY_PRESSED;
        }
    }
}

bool KeyTracker::IsHeld(int id) const {
    return isHeld[id];
}

}
