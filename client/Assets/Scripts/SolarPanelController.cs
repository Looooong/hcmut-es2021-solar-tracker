using ChartAndGraph;
using System;
using TMPro;
using UnityEngine;

public class SolarPanelController : MonoBehaviour
{
    static readonly Matrix4x4 SystemToWorld = new Matrix4x4(
        new Vector4(0f, 0f, 1f, 0f),
        new Vector4(-1f, 0f, 0f, 0f),
        new Vector4(0f, 1f, 0f, 0f),
        new Vector4(0f, 0f, 0f, 1f)
    );
    static readonly Matrix4x4 WorldToSystem = SystemToWorld.inverse;

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
    public Transform platformIndicator;

    [Header("Charts")]
    public TMP_Text solarPanelAzimuthValueText;
    public GraphChart solarPanelAzimuthChart;
    public TMP_Text solarPanelInclinationValueText;
    public GraphChart solarPanelInclinationChart;
    public TMP_Text azimuthMotorValueText;
    public GraphChart azimuthMotorChart;
    public TMP_Text inclinationMotorValueText;
    public GraphChart inclinationMotorChart;

    Orientation _lastOrientation;
    SystemState _currentState;
    Orientation _angularVelocity;

    public void AddSystemState(SystemState state)
    {
        var time = DateTimeOffset.FromUnixTimeMilliseconds(state.timestamp).UtcDateTime;
        solarPanelAzimuthChart.DataSource.AddPointToCategoryRealtime("Solar Panel Azimuth", time, state.panelOrientation.azimuth, .1f);
        solarPanelInclinationChart.DataSource.AddPointToCategoryRealtime("Solar Panel Inclination", time, state.panelOrientation.inclination, .1f);
        azimuthMotorChart.DataSource.AddPointToCategoryRealtime("Solar Panel Azimuth", time, state.motorsRotation.azimuth, .1f);
        inclinationMotorChart.DataSource.AddPointToCategoryRealtime("Solar Panel Inclination", time, state.motorsRotation.inclination, .1f);
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
        var nextOrientation = new Orientation
        {
            azimuth = Mathf.SmoothDampAngle(_lastOrientation.azimuth, _currentState.motorsRotation.azimuth, ref _angularVelocity.azimuth, smoothTime),
            inclination = Mathf.SmoothDampAngle(_lastOrientation.inclination, _currentState.motorsRotation.inclination, ref _angularVelocity.inclination, smoothTime),
        };
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

        solarPanelAzimuthValueText.text = _currentState.panelOrientation.azimuth.ToString("0.000°");
        solarPanelInclinationValueText.text = _currentState.panelOrientation.inclination.ToString("0.000°");
        azimuthMotorValueText.text = _currentState.motorsRotation.azimuth.ToString("0.000°");
        inclinationMotorValueText.text = _currentState.motorsRotation.inclination.ToString("0.000°");

        if (isManual)
        {
            azimuthInputField.SetTextWithoutNotify(controlConfig.manualOrientation.azimuth.ToString());
            inclinationInputField.SetTextWithoutNotify(controlConfig.manualOrientation.inclination.ToString());
            azimuthSlider.SetValue(controlConfig.manualOrientation.azimuth, false);
            inclinationSlider.SetValue(controlConfig.manualOrientation.inclination, false);
        }
        else
        {
            azimuthInputField.SetTextWithoutNotify(_currentState.panelOrientation.azimuth.ToString());
            inclinationInputField.SetTextWithoutNotify(_currentState.panelOrientation.inclination.ToString());
        }

        platformIndicator.rotation = (
            SystemToWorld *
            Matrix4x4.Rotate(_currentState.platformRotation) *
            WorldToSystem
        ).rotation;
    }

    float Mod(float a, float n)
    {
        return a - n * Mathf.FloorToInt(a / n);
    }
}
