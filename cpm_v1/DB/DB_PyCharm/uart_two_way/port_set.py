
import serial
import serial.tools.list_ports




# STM32 UART 속도 설정
BAUD_RATE = 115200  # STM32와 동일하게 설정

# 시스템에 연결된 모든 시리얼 포트 확인 (STMicroelectronics STLink Virtual COM Port 찾기)
def find_stm32_port():
    ports = list(serial.tools.list_ports.comports())
    for port in ports:
        print(f"포트 이름: {port.device}, 설명: {port.description}")
        # 포트 설명에 'STMicroelectronics STLink Virtual COM Port' 포함 확인
        if "STMicroelectronics STLink Virtual COM Port" in port.description:
            return port.device
    return None

# 시스템에 연결된 모든 시리얼 포트 확인 (XR21V1410 USB UART 포트 찾기)
def find_xr21v1410_port():
    ports = list(serial.tools.list_ports.comports())
    for port in ports:
        print(f"포트 이름: {port.device}, 설명: {port.description}")
        # 포트 설명에 'XR21V1410 USB UART' 포함 확인
        if "XR21V1410 USB UART" in port.description:
            return port.device
    return None

# 시리얼 포트 열기
def open_serial_port(port):
    try:
        ser = serial.Serial(port, BAUD_RATE, timeout=1)
        if ser.is_open:
            print(f"Serial port {port} is open.")
        return ser
    except serial.SerialException as e:
        print(f"Error opening serial port {port}: {e}")
        return None
