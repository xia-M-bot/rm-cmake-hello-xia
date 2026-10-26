extends Node

@export var img_port: int = 9000
@export var state_port: int = 9001
@export var infantry: Node3D
@export var viewport: SubViewport

var img_socket: StreamPeerTCP
var state_tcp: StreamPeerTCP
var send_timer: float = 0.0


func _ready():
	img_socket = StreamPeerTCP.new()
	state_tcp = StreamPeerTCP.new()
	img_socket.connect_to_host("127.0.0.1", img_port)
	state_tcp.connect_to_host("127.0.0.1", state_port)
	print("=== TcpSender 开始执行 ===")


func _process(delta):
	img_socket.poll()
	state_tcp.poll()

	if state_tcp.get_status() == StreamPeerTCP.STATUS_CONNECTED:
		var info = "hp:%d,ammo:%d\n" % [infantry.hp, infantry.ammo]
		state_tcp.put_data(info.to_utf8_buffer())

	send_timer += delta
	if send_timer < 0.1:
		return
	send_timer = 0.0

	if img_socket.get_status() == StreamPeerTCP.STATUS_CONNECTED:
		send_frame()


func send_frame():
	await RenderingServer.frame_post_draw
	var tex = get_tree().root.get_texture()
	if tex == null:
		return
	var img = tex.get_image()
	if img == null:
		return

	var png_data = img.save_png_to_buffer()
	if png_data.size() == 0:
		return

	var packet = PackedByteArray()
	packet.resize(4)
	packet.encode_u32(0, png_data.size())
	packet.append_array(png_data)
	img_socket.put_data(packet)

	# 发送状态（这个你已经跑通了，保留）
	if state_tcp.get_status() == StreamPeerTCP.STATUS_CONNECTED:
		var info = "hp:%d,ammo:%d\n" % [infantry.hp, infantry.ammo]
		state_tcp.put_data(info.to_utf8_buffer())
		print("发送状态：", info)

	# 发送图像
	if img_socket.get_status() == StreamPeerTCP.STATUS_CONNECTED:
		if tex == null:
			print("tex为空")
			return
		if img == null:
			print("img为空")
			return
		print("图片 w=", img.get_width(), " h=", img.get_height())
		# 先测试：保存图片到项目目录，验证能不能拿到画面！
		print("图片字节长度：", png_data.size())
		packet.resize(4)
		packet.encode_u32(0,png_data.size())
		packet.append_array(png_data)
		img_socket.put_data(packet)
