[System.Serializable]
public struct ControlConfig
{
    public ControlConfigMode controlMode;
    public Orientation manualOrientation;
}

public enum ControlConfigMode
{
    AUTOMATIC,
    MANUAL,
}
