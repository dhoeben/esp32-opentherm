#pragma once
namespace opentherm {
class OpenThermComponent;
namespace Diagnostics {
void update(OpenThermComponent *ot);
bool has_fault();
void clear_faults();
} // namespace Diagnostics
} // namespace opentherm
