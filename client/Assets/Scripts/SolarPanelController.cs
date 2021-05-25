using System;
using TMPro;
using UnityEngine;

public class SolarPanelController : MonoBehaviour
{
    public ControlConfig controlConfig;
    public CloudClient cloudClient;

    [Header("UI")]
    public TMP_Dropdown modeDropdown;
    public TMP_InputField azimuthInputField;
    public TMP_InputField inclinationInputField;

    public void UpdateConfig(ControlConfig config)
    {
        controlConfig = config;
        modeDropdown.SetValueWithoutNotify((int)config.controlMode);
        azimuthInputField.SetTextWithoutNotify(config.manualOrientation.azimuth.ToString());
        inclinationInputField.SetTextWithoutNotify(config.manualOrientation.inclination.ToString());
    }

    void Awake()
    {
        modeDropdown.onValueChanged.AddListener(value =>
        {
            controlConfig.controlMode = (ControlConfigMode)value;
            cloudClient.UpdateConfig(controlConfig);
        });
        azimuthInputField.onEndEdit.AddListener(value =>
        {
            controlConfig.manualOrientation.azimuth = Convert.ToSingle(value);
            cloudClient.UpdateConfig(controlConfig);
        });
        inclinationInputField.onEndEdit.AddListener(value =>
        {
            controlConfig.manualOrientation.inclination = Convert.ToSingle(value);
            cloudClient.UpdateConfig(controlConfig);
        });
    }
}
