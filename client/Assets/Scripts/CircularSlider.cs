using TMPro;
using UnityEngine;
using UnityEngine.EventSystems;
using UnityEngine.UI;

public class CircularSlider : Selectable, IInitializePotentialDragHandler, IDragHandler
{
    [Header("Circular Slider")]
    public RectTransform handleRect;
    public TMP_Text valueText;
    public Slider.SliderEvent onValueChanged;
    [Range(0f, 360f)]
    public float minValue = 0f;
    [Range(0f, 360f)]
    public float maxValue = 360f;
    public float value;

    protected override void Awake()
    {
        base.Awake();

        SetValue(value, false);
    }

    public override void OnPointerDown(PointerEventData eventData)
    {
        base.OnPointerDown(eventData);

        OnDrag(eventData);
    }

    public void OnInitializePotentialDrag(PointerEventData eventData)
    {
        eventData.useDragThreshold = false;
    }

    public void OnDrag(PointerEventData eventData)
    {
        if (!IsActive() || !IsInteractable() || eventData.button != PointerEventData.InputButton.Left) return;

        HandleDrag(eventData);
    }

    protected override void OnValidate()
    {
        base.OnValidate();

        SetValue(value, false);
    }

    public void SetValue(float newValue, bool notify)
    {
        value = Mathf.Clamp(newValue, minValue, maxValue);
        valueText.text = value.ToString("0.0");
        handleRect.localRotation = Quaternion.identity;
        handleRect.Rotate(0f, 0f, -value);

        if (notify) onValueChanged.Invoke(value);
    }

    void HandleDrag(PointerEventData eventData)
    {
        var ray = Camera.main.ScreenPointToRay(eventData.position);
        var plane = new Plane(transform.forward, transform.position);

        plane.Raycast(ray, out var distance);

        var newValue = -Vector3.SignedAngle(transform.up, ray.GetPoint(distance) - transform.position, transform.forward);

        if (newValue < 0f && newValue + 360f <= maxValue) newValue += 360f;

        SetValue(newValue, true);
    }
}
