local Input = {}

function Input.GetAxis(Axis)
    return GetAxis_Internal(Axis)
end

function Input.IsKeyDown(key)
    return IsKeyDown_Internal(key)
end

return Input