using UnityEngine;

public class FaceCamera : MonoBehaviour
{
    public bool rotate = true;
    public Vector3 flipScale = new Vector3(1f, -1f, 1f);

    Camera _camera;
    RectTransform _rectTransform;

    void Awake()
    {
        _camera = Camera.main;
        _rectTransform = (RectTransform)transform;
    }

    void Update()
    {
        if (rotate)
        {
            transform.Rotate(0f, Vector3.SignedAngle(Vector3.ProjectOnPlane(_camera.transform.forward, transform.forward), transform.up, transform.forward), 0f, Space.World);
        }

        transform.localScale = Vector3.Dot(_camera.transform.forward, transform.forward) >= 0f ? Vector3.one : flipScale;
    }
}
