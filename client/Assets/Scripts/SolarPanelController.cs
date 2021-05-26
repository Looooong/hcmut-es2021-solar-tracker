using ChartAndGraph;
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
    public CircularSlider azimuthSlider;
    public CircularSlider inclinationSlider;

    [Header("Charts")]
    public GraphChart solarPanelVoltageChart;

    SystemState _currentState;

    public void AddSystemState(SystemState state)
    {
        var time = DateTimeOffset.FromUnixTimeMilliseconds((long)(state.timestamp * 1000)).UtcDateTime;
        solarPanelVoltageChart.DataSource.AddPointToCategoryRealtime("Solar Panel Voltage", time, state.solarPanelVoltage, 1f);
        _currentState = state;
        UpdateUI();
    }

    public void UpdateControlConfig(ControlConfig config)
    {
        controlConfig = config;
        UpdateUI();
    }

    void Awake()
    {
        modeDropdown.onValueChanged.AddListener(value =>
        {
            controlConfig.controlMode = (ControlConfigMode)value;
            cloudClient.UpdateConfig(controlConfig);
            UpdateUI();
        });
        azimuthInputField.onEndEdit.AddListener(value =>
        {
            controlConfig.manualOrientation.azimuth = Mod(Convert.ToSingle(value), 360f);
            cloudClient.UpdateConfig(controlConfig);
            UpdateUI();
        });
        inclinationInputField.onEndEdit.AddListener(value =>
        {
            controlConfig.manualOrientation.inclination = Mod(Convert.ToSingle(value), 360f);
            cloudClient.UpdateConfig(controlConfig);
            UpdateUI();
        });
        azimuthSlider.onValueChanged.AddListener(value =>
        {
            controlConfig.manualOrientation.azimuth = value;
            cloudClient.UpdateConfig(controlConfig);
            UpdateUI();
        });
        inclinationSlider.onValueChanged.AddListener(value =>
        {
            controlConfig.manualOrientation.inclination = value;
            cloudClient.UpdateConfig(controlConfig);
            UpdateUI();
        });

        UpdateUI();
    }

    void UpdateUI()
    {
        var isManual = controlConfig.controlMode == ControlConfigMode.MANUAL;

        azimuthInputField.interactable = isManual;
        inclinationInputField.interactable = isManual;
        azimuthSlider.interactable = isManual;
        inclinationSlider.interactable = isManual;

        modeDropdown.SetValueWithoutNotify((int)controlConfig.controlMode);

        if (isManual)
        {
            azimuthInputField.SetTextWithoutNotify(controlConfig.manualOrientation.azimuth.ToString());
            inclinationInputField.SetTextWithoutNotify(controlConfig.manualOrientation.inclination.ToString());
            azimuthSlider.SetValue(controlConfig.manualOrientation.azimuth, false);
            inclinationSlider.SetValue(controlConfig.manualOrientation.inclination, false);
        }
        else
        {
            azimuthInputField.SetTextWithoutNotify(_currentState.solarPanelOrientation.azimuth.ToString());
            inclinationInputField.SetTextWithoutNotify(_currentState.solarPanelOrientation.inclination.ToString());
            azimuthSlider.SetValue(_currentState.solarPanelOrientation.azimuth, false);
            inclinationSlider.SetValue(_currentState.solarPanelOrientation.inclination, false);
        }
    }

    float Mod(float a, float n)
    {
        return a - n * Mathf.FloorToInt(a / n);
    }
}
