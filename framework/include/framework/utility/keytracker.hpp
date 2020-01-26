#ifndef FRAMEWORK_UTILITY_KEYTRACKER_HPP
#define FRAMEWORK_UTILITY_KEYTRACKER_HPP

#include <framework/event.hpp>

#include <vector>

namespace zfw {

class KeyTracker {
public:
    KeyTracker(IVarSystem* varSystem);

    // Choose any unused, non-negative ID that you like; ideally map your subscription IDs in an enum, if feasible
    void Subscribe(int id, const char* bindingName, Vkey_t defaultKey);

    void OnFrame();
    void Handle(EventVkey const& ev); // TODO API: this might be better handled through PubSub subscription
    bool IsHeld(int id) const;
    // bool WasPressed(int id) const;

private:
    std::vector<Vkey_t> trackedKeys;
    std::vector<bool> isHeld;
    // std::vector<bool> wasPressed;
};

}

#endif
