using UnityEngine;

[System.Serializable]
public struct SystemState
{
    public long timestamp;
    public float solarPanelCurrent;
    public float solarPanelVoltage;
    public float batteryVoltage;
    public Orientation solarPanelOrientation;
    public Orientation motorsRotation;
    public Quaternion platformRotation;
}
