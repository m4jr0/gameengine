#include "event.h"

namespace comet {
namespace event {
bool Event::IsInCategory(EventCategory category) const {
  return GetCategoryFlags() & category;
}
}  // namespace event
}  // namespace comet