using UnityEngine;

[System.Serializable]
public struct SystemState
{
    public Quaternion platformRotation;
    public Orientation panelOrientation;
    public Orientation motorsRotation;
    public long timestamp;
}
