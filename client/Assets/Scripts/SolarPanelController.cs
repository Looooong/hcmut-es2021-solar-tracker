using ChartAndGraph;
using System;
using TMPro;
using UnityEngine;

public class SolarPanelController : MonoBehaviour
{
    public ControlConfig controlConfig;
    public CloudClient cloudClient;

    [Header("Motors")]
    public Transform motor1;
    public Transform motor2;
    public float smoothTime = .1f;

    [Header("UI")]
    public TMP_Dropdown modeDropdown;
    public TMP_InputField azimuthInputField;
    public TMP_InputField inclinationInputField;
    public CircularSlider azimuthSlider;
    public CircularSlider inclinationSlider;

    [Header("Charts")]
    public TMP_Text solarPanelVoltageValueText;
    public GraphChart solarPanelVoltageChart;
    public TMP_Text solarPanelAzimuthValueText;
    public GraphChart solarPanelAzimuthChart;
    public TMP_Text solarPanelInclinationValueText;
    public GraphChart solarPanelInclinationChart;

    Orientation _lastOrientation;
    SystemState _currentState;
    Orientation _angularVelocity;

    public void AddSystemState(SystemState state)
    {
        var time = DateTimeOffset.FromUnixTimeMilliseconds(state.timestamp).UtcDateTime;
        solarPanelVoltageChart.DataSource.AddPointToCategoryRealtime("Solar Panel Voltage", time, state.solarPanelVoltage, .1f);
        solarPanelAzimuthChart.DataSource.AddPointToCategoryRealtime("Solar Panel Azimuth", time, state.solarPanelOrientation.azimuth, .1f);
        solarPanelInclinationChart.DataSource.AddPointToCategoryRealtime("Solar Panel Inclination", time, state.solarPanelOrientation.inclination, .1f);
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

    void Update()
    {
        var nextOrientation = new Orientation();
        nextOrientation.azimuth = Mathf.SmoothDampAngle(_lastOrientation.azimuth, _currentState.solarPanelOrientation.azimuth, ref _angularVelocity.azimuth, smoothTime);
        nextOrientation.inclination = Mathf.SmoothDampAngle(_lastOrientation.inclination, _currentState.solarPanelOrientation.inclination, ref _angularVelocity.inclination, smoothTime);
        motor1.Rotate(0f, nextOrientation.azimuth - _lastOrientation.azimuth, 0f);
        motor2.Rotate(0f, nextOrientation.inclination - _lastOrientation.inclination, 0f);
        _lastOrientation = nextOrientation;
        _lastOrientation.azimuth = Mod(_lastOrientation.azimuth, 360f);
        _lastOrientation.inclination = Mod(_lastOrientation.inclination, 360f);

        if (controlConfig.controlMode == ControlConfigMode.AUTOMATIC)
        {
            azimuthSlider.SetValue(_lastOrientation.azimuth, false);
            inclinationSlider.SetValue(_lastOrientation.inclination, false);
        }
    }

    void UpdateUI()
    {
        var isManual = controlConfig.controlMode == ControlConfigMode.MANUAL;

        azimuthInputField.interactable = isManual;
        inclinationInputField.interactable = isManual;
        azimuthSlider.interactable = isManual;
        inclinationSlider.interactable = isManual;

        modeDropdown.SetValueWithoutNotify((int)controlConfig.controlMode);

        solarPanelVoltageValueText.text = _currentState.solarPanelVoltage.ToString("0.000V");
        solarPanelAzimuthValueText.text = _currentState.solarPanelOrientation.azimuth.ToString("0.0°");
        solarPanelInclinationValueText.text = _currentState.solarPanelOrientation.inclination.ToString("0.0°");

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
        }
    }

    float Mod(float a, float n)
    {
        return a - n * Mathf.FloorToInt(a / n);
    }
}
