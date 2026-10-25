extends Node

@export var img_port: int = 9000
@export var state_port: int = 9001
@export var viewport: SubViewport
@export var infantry: Node3D

var img_tcp: StreamPeerTCP
var state_tcp: StreamPeerTCP
var send_timer: Timer


func _ready():
	print("==== TcpSender 开始执行 ====")
	img_tcp = StreamPeerTCP.new()
	state_tcp = StreamPeerTCP.new()

	var ret1 = img_tcp.connect_to_host("127.0.0.1", img_port)
	var ret2 = state_tcp.connect_to_host("127.0.0.1", state_port)
	print("图像连接返回：", ret1)
	print("状态连接返回：", ret2)

	send_timer = Timer.new()
	send_timer.wait_time = 0.05
	send_timer.autostart = true
	send_timer.timeout.connect(_send_data)
	add_child(send_timer)


func _send_data():
	# 先检查有没有连上
	if img_tcp.get_status() != StreamPeerTCP.STATUS_CONNECTED:
		return
	if state_tcp.get_status() != StreamPeerTCP.STATUS_CONNECTED:
		return

	# 发送状态文本
	var info = "hp:%f,ammo:%d" % [infantry.hp, infantry.ammo]
	state_tcp.put_data(info.to_utf8_buffer())

	# 截图发给python
	var img: Image = viewport.get_texture().get_image()
	img.convert(Image.FORMAT_RGB8)
	var buf: PackedByteArray = img.save_to_buffer("png")
	img_tcp.put_32(buf.size())
	img_tcp.put_data(buf)
