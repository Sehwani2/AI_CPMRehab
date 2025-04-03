import serial

# 오브젝트 값들을 저장할 전역 변수들
speed = 0
angle = 0
repeat_count = 0
start_button = False

#받아온 값을 정수로 저장해서 보내는



# UART 데이터를 읽어서 처리하고 값들을 저장하는 함수
def read_uart_data(port):
    global speed, angle, repeat_count, start_button

    while True:
        try:
            if port.in_waiting > 0:  # UART로 읽을 데이터가 있는 경우
                data = port.read(port.in_waiting)  # 데이터를 가능한 한 모두 읽기
                formatted_data = " ".join([f"{byte:02X}" for byte in data])  # 바이트를 16진수 형식으로 변환하여 출력
                print(f"수신된 데이터: {formatted_data}")  # 데이터 출력

                # 데이터를 배열로 나누어 분석
                data_array = [byte for byte in data]

                if len(data_array) >= 9:  # 최소 9바이트 이상일 경우만 처리
                    master_byte = data_array[0:2]  # 마스터 식별 바이트
                    command_byte = data_array[3]  # 명령 바이트
                    address_bytes = data_array[4:6]  # 오브젝트 주소 (2바이트)
                    address = (address_bytes[0] << 8) | address_bytes[1]  # 오브젝트 주소 값

                    # 마스터 식별 및 명령 확인
                    if master_byte == [0x5A, 0xA5]:  # 마스터 식별
                        if command_byte == 0x83:  # 83은 받기 명령
                            if address_bytes == [0x10, 0x00]:  # 속도 (주소: 0x1000)
                                speed = (data_array[7] << 8) | data_array[8]
                                print(f"속도: {speed}")
                            elif address_bytes == [0x20, 0x00]:  # 각도 (주소: 0x2000)
                                angle = (data_array[7] << 8) | data_array[8]
                                print(f"각도: {angle}")
                            elif address_bytes == [0x30, 0x00]:  # 반복횟수 (주소: 0x3000)
                                repeat_count = (data_array[7] << 8) | data_array[8]
                                print(f"반복 횟수: {repeat_count}")
                            elif address_bytes == [0x40, 0x00]:  # 스타트 버튼 (주소: 0x4000)
                                start_button = True
                                print("스타트 버튼 눌림!")

                # 스타트 버튼이 눌리면 저장된 값 출력
                if start_button:
                    print(f"현재 상태 - 속도: {speed}, 각도: {angle}, 반복 횟수: {repeat_count}")
                    start_button = False  # 스타트 버튼을 누른 후 상태 출력 후 다시 초기화

        except Exception as e:
            print(f"UART 데이터 읽기 오류: {e}")
            break


# 메인 코드
try:
    # UART 포트 설정 (COM4 예시)
    uart_port = serial.Serial('COM4', baudrate=115200, timeout=1)
    print("UART 포트 연결 성공")

    # UART 데이터 읽기
    read_uart_data(uart_port)

except serial.SerialException as e:
    print(f"시리얼 연결 오류: {e}")

except Exception as e:
    print(f"오류 발생: {e}")

finally:
    # 포트 닫기
    if 'uart_port' in locals() and uart_port.is_open:
        uart_port.close()
    print("UART 포트 닫힘")
