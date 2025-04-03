import serial
import platform
import serial.tools.list_ports


# UART 포트 이름으로 검색하는 함수
def get_uart_port_by_name(port_name):
    ports = serial.tools.list_ports.comports()
    for port in ports:
        if port_name in port.description:
            return port.device
    return None


# UART 데이터를 읽어서 처리하고 값을 STM32로 보내는 함수
def read_uart_data(port_in, port_out):
    while True:
        try:
            if port_in.in_waiting > 0:  # UART로 읽을 데이터가 있는 경우
                data = port_in.read(port_in.in_waiting)  # 데이터를 가능한 한 모두 읽기
                formatted_data = " ".join([f"{byte:02X}" for byte in data])  # 바이트를 16진수 형식으로 변환
                print(f"수신된 데이터: {formatted_data}")  # 데이터 출력

                # 데이터를 STM32로 전송
                port_out.write(data)  # STM32로 전송
                print(f"송신된 데이터: {formatted_data}")  # 송신 데이터 확인

        except Exception as e:
            print(f"UART 데이터 처리 오류: {e}")
            break


# 메인 코드
try:
    # 운영체제 확인
    os_type = platform.system()

    # 윈도우에서 포트를 이름으로 찾기
    if os_type == "Windows":
        uart_port_in_name = "XR21V1410 USB UART"
        uart_port_out_name = "STMicroelectronics STLink Virtual COM Port"

        uart_port_in = get_uart_port_by_name(uart_port_in_name)
        uart_port_out = get_uart_port_by_name(uart_port_out_name)

        if uart_port_in is None or uart_port_out is None:
            raise serial.SerialException("포트를 찾을 수 없습니다.")

        uart_port_in = serial.Serial(uart_port_in, baudrate=115200, timeout=1)
        uart_port_out = serial.Serial(uart_port_out, baudrate=115200, timeout=1)

    # 리눅스에서 포트를 이름으로 찾기
    elif os_type == "Linux":
        uart_port_in_name = "XR21V1410 USB UART"
        uart_port_out_name = "STMicroelectronics STLink Virtual COM Port"

        uart_port_in = get_uart_port_by_name(uart_port_in_name)
        uart_port_out = get_uart_port_by_name(uart_port_out_name)

        if uart_port_in is None or uart_port_out is None:
            raise serial.SerialException("포트를 찾을 수 없습니다.")

        uart_port_in = serial.Serial(uart_port_in, baudrate=115200, timeout=1)
        uart_port_out = serial.Serial(uart_port_out, baudrate=115200, timeout=1)

    else:
        raise Exception("지원하지 않는 운영체제입니다.")

    print("UART 포트 연결 성공")

    # 입력 포트 및 출력 포트 연결 확인
    if uart_port_in.is_open:
        print(f"입력 포트 {uart_port_in.portstr} 연결됨")
    else:
        raise serial.SerialException("입력 포트 연결 실패")

    if uart_port_out.is_open:
        print(f"출력 포트 {uart_port_out.portstr} 연결됨")
    else:
        raise serial.SerialException("출력 포트 연결 실패")

    # UART 데이터 읽고 보내기
    read_uart_data(uart_port_in, uart_port_out)

except serial.SerialException as e:
    print(f"시리얼 연결 오류: {e}")

except Exception as e:
    print(f"오류 발생: {e}")

finally:
    # 포트 닫기
    if 'uart_port_in' in locals() and uart_port_in.is_open:
        uart_port_in.close()
        print(f"입력 포트 {uart_port_in.portstr} 닫힘")

    if 'uart_port_out' in locals() and uart_port_out.is_open:
        uart_port_out.close()
        print(f"출력 포트 {uart_port_out.portstr} 닫힘")

    print("UART 포트 닫힘")
