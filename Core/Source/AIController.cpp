#include "AIController.h"

#include "Object.h"

using namespace Apex::Controller;

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
