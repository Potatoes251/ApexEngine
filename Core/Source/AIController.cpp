#include "AIController.h"

#include "Object.h"

#include <sstream>

using namespace Apex::Controller;

void AIController::Serialize(std::ostream& out) const
{
    std::stringstream temp;
    CharacterController::Serialize(temp);

    std::string baseContent = temp.str();

    if (!baseContent.empty() && baseContent.back() == '\n') 
    {
        out << baseContent.substr(0, baseContent.length() - 1);
        out << ",\n";
    }
    else 
    {
        out << baseContent;
        out << ",\n";
    }

    out << "\"arrivalThreshold\" :" << m_arrivalThreshold << "\n";
}

std::vector<Apex::ExposedVar> Apex::Controller::AIController::GetExposedVariables()
{
    std::vector<ExposedVar> var = CharacterController::GetExposedVariables();
    var.push_back({ "Arrival Threshold", ExposedVar::Float, &m_arrivalThreshold });

    return var;
}

void AIController::OnFixedUpdate(float deltatime_s)
{
    if (m_target)
    {
        LibMath::Vector3 toTarget = m_target->GetGlobalPosition() - m_targetPos;
        toTarget[1] = 0.f;

        float distSq = toTarget.magnitudeSquared();

        if (distSq > m_arrivalThreshold * m_arrivalThreshold)
        {
            Move(toTarget.normalized());
        }
        else
        {
            m_target = nullptr;
        }
    }

	CharacterController::OnFixedUpdate(deltatime_s);
}
