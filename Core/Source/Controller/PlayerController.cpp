#include "Controller/PlayerController.h"

using namespace Apex::Controller;


void PlayerController::Serialize(std::ostream& out) const
{
    out << "      \"PlayerController\": {\n";
    out << "        \"enabled\": " << (IsEnabled() ? "true" : "false") << ",\n";

    out << "      }";
}
