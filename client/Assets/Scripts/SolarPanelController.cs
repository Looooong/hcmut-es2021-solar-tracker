using UnityEngine;

public class SolarPanelController : MonoBehaviour
{
    public ControlConfig controlConfig;

    public void UpdateConfig(ControlConfig config)
    {
        controlConfig = config;
    }
}
