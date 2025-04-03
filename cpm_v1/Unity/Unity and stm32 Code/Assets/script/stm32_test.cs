using UnityEngine;
using System.IO.Ports;

public class stm32_test : MonoBehaviour
{
    SerialPort serial;
    public GameObject Cube; // 회전시킬 큐브 오브젝트

    private float midpoint = 2048.0f; // 가변저항 값의 중위값 (40 ~ 4085 기준에서 중간값)
    private float maxADC = 4085.0f;  // 가변저항 값의 최대값
    private float speedMultiplier = 500.0f; // 회전 속도 계수 (값을 크게 해서 속도 증가)

    private void Awake()
    {
        serial = new SerialPort("COM6", 115200); // STM32 포트와 동일한 설정
        serial.Open();
    }

    void Update()
    {
        if (serial.IsOpen)
        {
            try
            {
                // STM32에서 데이터를 읽어옵니다
                string data = serial.ReadLine();
                Debug.Log("Received: " + data);

                // 데이터를 float 값으로 변환
                if (float.TryParse(data, out float adcValue))
                {
                    RotateCube(adcValue); // 큐브 회전 함수 호출
                }
            }
            catch (System.Exception e)
            {
                Debug.LogError(e.Message);
            }
        }
    }

    /// <summary>
    /// 큐브를 가변저항 값에 따라 회전시킵니다.
    /// </summary>
    /// <param name="adcValue">가변저항 값 (40 ~ 4085)</param>
    private void RotateCube(float adcValue)
    {
        // 중위값과의 거리 계산
        float distanceFromMidpoint = Mathf.Abs(adcValue - midpoint);

        // 회전 속도 계산 (중위값에서 멀어질수록 빠르게 회전)
        float rotationSpeed = (distanceFromMidpoint / maxADC) * speedMultiplier;

        // 회전 방향 결정 (중위값보다 작으면 왼쪽, 크면 오른쪽)
        if (adcValue < midpoint)
        {
            rotationSpeed = -rotationSpeed; // 왼쪽 회전
        }

        // 큐브 회전 적용
        Cube.transform.Rotate( rotationSpeed * Time.deltaTime, 0, 0);
    }

    private void OnApplicationQuit()
    {
        if (serial.IsOpen)
            serial.Close();
    }
}
