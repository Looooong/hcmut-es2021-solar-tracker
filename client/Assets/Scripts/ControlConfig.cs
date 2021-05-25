using Newtonsoft.Json;
using Newtonsoft.Json.Converters;

[System.Serializable]
public struct ControlConfig
{
    [JsonConverter(typeof(StringEnumConverter))]
    public ControlConfigMode controlMode;
    public Orientation manualOrientation;
}

public enum ControlConfigMode
{
    AUTOMATIC,
    MANUAL,
}
