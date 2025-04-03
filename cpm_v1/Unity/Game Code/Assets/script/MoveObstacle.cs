using UnityEngine;
using UnityEngine.SocialPlatforms.Impl;

public class MoveObstacle : MonoBehaviour
{
    public float moveSpeed = 2f;  // 벽 이동 속도

    private ObstacleSpawner Managerobj;


    private void Start()
    {
        // "ObstacleSpawner"라는 이름의 오브젝트를 찾아서 스크립트 가져오기
        GameObject Manager = GameObject.Find("Manager");

        if (Manager != null)
        {
            Managerobj = Manager.GetComponent<ObstacleSpawner>();
        }
        else
        {
            Debug.LogError("Manager 오브젝트를 찾을 수 없습니다!");
        }
    }



    void Update()
    {
        transform.position += Vector3.left * moveSpeed * Time.deltaTime; // 왼쪽으로 이동

        // 화면 밖으로 나가면 제거
        if (transform.position.x < -10f)
        {
            Debug.Log("회피 성공");

            Managerobj.IncreaseScore();


            if (transform.parent != null)
            {
                Destroy(transform.parent.gameObject);
            }
            else
            {
                Destroy(gameObject); // 부모가 없으면 자기 자신 삭제
            }
        }
    }


    void OnTriggerEnter2D(Collider2D collision)
    {
        Debug.Log("벽과 충돌");

        Managerobj.minusScore();


        if (transform.parent != null)
        {
            Destroy(transform.parent.gameObject);
        }
        else
        {
            Destroy(gameObject); // 부모가 없으면 자기 자신 삭제
        }

    }

   
}
