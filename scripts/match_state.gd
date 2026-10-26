extends Node3D

# 比赛总时长（秒），RM 常规是 180 秒 = 3 分钟
@export var match_time: int = 180

# 剩余时间
var time_left: int

# 比赛是否结束
var game_over: bool = false

signal time_changed(seconds: int)
signal game_started()
signal game_ended()

func _ready():
	time_left = match_time
	game_over = false
	# 发一个初始信号，让HUD立刻显示时间
	time_changed.emit(time_left)
	game_started.emit()

func _process(delta):
	if game_over:
		return

	time_left -= delta

	if time_left <= 0:
		time_left = 0
		game_over = true
		time_changed.emit(time_left)
		game_ended.emit()
		return

	# 只在秒数变化时发信号，避免每帧都刷文字
	var whole_sec = int(time_left)
	# 这里我们简单每帧发，HUD自己处理格式也行
	time_changed.emit(whole_sec)
