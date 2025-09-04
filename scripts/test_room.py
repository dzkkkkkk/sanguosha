import socket
import struct
import time
import sanguosha_pb2

def send_msg(sock, msg):
    body = msg.SerializeToString()
    header = struct.pack("!I", len(body))
    sock.sendall(header + body)

def recv_msg(sock):
    header = sock.recv(4)
    body_size = struct.unpack("!I", header)[0]
    body = sock.recv(body_size)
    return body

def test_room_operations():
    host = 'localhost'
    port = 9527
    
    print("=== 房间管理系统测试 ===")
    
    # 玩家1：创建房间
    sock1 = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock1.connect((host, port))
    
    # 登录
    login_msg = sanguosha_pb2.GameMessage()
    login_msg.type = sanguosha_pb2.LOGIN_REQUEST
    login_msg.login_request.username = "player1"
    login_msg.login_request.password = "123"
    send_msg(sock1, login_msg)
    recv_msg(sock1)  # 登录响应
    
    # 创建房间
    room_msg = sanguosha_pb2.GameMessage()
    room_msg.type = sanguosha_pb2.ROOM_REQUEST
    room_msg.room_request.action = sanguosha_pb2.CREATE_ROOM
    send_msg(sock1, room_msg)
    create_resp = sanguosha_pb2.GameMessage()
    create_resp.ParseFromString(recv_msg(sock1))
    room_id = create_resp.room_response.room_info.room_id
    print(f"玩家1创建房间: {room_id}")
    
    # 玩家2：加入房间
    sock2 = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock2.connect((host, port))
    
    login_msg.login_request.username = "player2"
    send_msg(sock2, login_msg)
    recv_msg(sock2)  # 登录响应
    
    join_msg = sanguosha_pb2.GameMessage()
    join_msg.type = sanguosha_pb2.ROOM_REQUEST
    join_msg.room_request.action = sanguosha_pb2.JOIN_ROOM
    join_msg.room_request.room_id = room_id
    send_msg(sock2, join_msg)
    join_resp = sanguosha_pb2.GameMessage()
    join_resp.ParseFromString(recv_msg(sock2))
    print(f"玩家2加入房间结果: {join_resp.room_response.success}")
    
    # 玩家1：开始游戏
    start_msg = sanguosha_pb2.GameMessage()
    start_msg.type = sanguosha_pb2.ROOM_REQUEST
    start_msg.room_request.action = sanguosha_pb2.START_GAME
    start_msg.room_request.room_id = room_id
    send_msg(sock1, start_msg)
    start_resp = sanguosha_pb2.GameMessage()
    start_resp.ParseFromString(recv_msg(sock1))
    print(f"开始游戏结果: {start_resp.room_response.success}")
    
    # 关闭连接
    sock1.close()
    sock2.close()
    print("测试完成")

if __name__ == '__main__':
    test_room_operations()