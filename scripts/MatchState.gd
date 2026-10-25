extends Node

@export var match_duration: float = 180.0 #总时长3分钟
var time_left: float = 0.0
var match_running: bool = false

signal time_updated(left_sec:float)

func _ready():
	begin_match()

func begin_match():
	time_left = match_duration
	match_running = true

func _physics_process(delta):
	if not match_running:
		return
	time_left = max(time_left - delta, 0.0)
	time_updated.emit(time_left)
	if time_left <= 0:
		match_running = false
