using UnityEngine;
using TMPro;



public class PlayerController : MonoBehaviour
{
    public float moveSpeed = 5f;
    public float maxHeight = 4.5f;
    public float minHeight = -4.5f;
    private Rigidbody2D rb;




    void Start()
    {
        rb = GetComponent<Rigidbody2D>();


    }

    void Update()
    {
       
        // 방향키로 위/아래 이동
        float moveInput = Input.GetAxis("Vertical");
        rb.linearVelocity = new Vector2(0, moveInput * moveSpeed);

        // 화면 범위 제한
        float clampedY = Mathf.Clamp(transform.position.y, minHeight, maxHeight);
        transform.position = new Vector3(transform.position.x, clampedY, 0);
    }

    // 벽과 충돌 시 게임 오버 처리
    void OnCollisionEnter2D(Collision2D other)
    {
        Debug.Log("충돌");

        
   
    }


}
