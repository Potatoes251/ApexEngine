#include "AIController.h"

#include "Object.h"
#include "Application.h"
#include "WaypointGraph.h"

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

void AIController::SetTarget(Data::Object* newTarget)
{ 
    if (newTarget == m_target) return;
    m_target = newTarget;

    if (m_target)
    {
        int current = m_graph->GetClosestWaypoint(m_targetPos);
        int target = m_graph->GetClosestWaypoint(m_target->GetGlobalPosition());

        SetPath(m_graph->GetPath(current, target));
    }
    else
    {
        // clear the path
        SetPath({});
    }
}

void AIController::OnStart()
{
    CharacterController::OnStart();
    m_graph = Application::Get()->GetScene()->GetGraph();
}

void AIController::OnFixedUpdate(float deltatime_s)
{
    m_pathRefreshTimer += deltatime_s;
    if (m_pathRefreshTimer >= m_pathRefreshInterval && m_target)
    {
        m_pathRefreshTimer = 0.f;
        int current = m_graph->GetClosestWaypoint(m_targetPos);
        int target = m_graph->GetClosestWaypoint(m_target->GetGlobalPosition());

        SetPath(m_graph->GetPath(current, target));
    }

    if (m_pathIdx < m_path.size())
    {
        LibMath::Vector3 toTarget = m_path[m_pathIdx] - m_targetPos;
        toTarget[1] = 0.f;

        float distSq = toTarget.magnitudeSquared();

        if (distSq > m_arrivalThreshold * m_arrivalThreshold)
        {
            Move(toTarget.normalized());
        }
        else
        {
            m_pathIdx++;
        }
    }

	CharacterController::OnFixedUpdate(deltatime_s);
}

void AIController::SetPath(std::vector<LibMath::Vector3> const& path)
{
    m_path = path;
    m_pathIdx = 0;
}
