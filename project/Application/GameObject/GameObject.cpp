#include "GameObject.h"

int GameObject::GetUpdateOrder() const
{
    int index = static_cast<int>(tag_);
    int count = static_cast<int>(ObjectTag::Count);
    if (index < 0 || index >= count) {
        return 0;
    }
    return kObjectUpdateOrder[index];
}
