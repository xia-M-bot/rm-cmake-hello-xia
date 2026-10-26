extends CanvasLayer

@onready var player = $"../Player"
@onready var armor = $"../ArmorFront"

# UI标签
@onready var hp_label = $HudRoot/RightPanel/HpLabel
@onready var ammo_label = $HudRoot/RightPanel/AmmoLabel
@onready var heat_label = $HudRoot/RightPanel/HeatLabel
@onready var hits_label = $HudRoot/RightPanel/HitsLabel
@onready var clock_label = $HudRoot/TopClockLabel
@onready var tip_label = $HudRoot/TipLabel

# 小地图
@onready var player_icon = $HudRoot/Minimap/PlayerIcon
var map_center := Vector2(100, 100)
var map_scale := 8.0

# 倒计时
var total_time := 60.0
var time_left := 60.0
var timer_running := true


func _process(delta):
	# 倒计时更新
	if timer_running:
		time_left -= delta
		if time_left <= 0:
			time_left = 0
			timer_running = false
			clock_label.text = "时间到！"
		else:
			clock_label.text = "剩余：%d s" % int(time_left)
	
	# HUD文字刷新（读取步兵脚本里的变量）
	if armor !=null:
		hp_label.text = "HP: %d" % player.hp
	else:
		hp_label.text = "HP: --"
	if player != null:
		ammo_label.text = "弹药: %d" % player.ammo
		heat_label.text = "热量: %d" % player.heat
		hits_label.text = "命中: %d" % player.hits
	else:
		ammo_label.text = "弹药: --"
		heat_label.text = "热量: --"
		hits_label.text = "命中: --"

	# 小地图图标跟随步兵
	if player != null and player_icon != null:
		var w_pos = player.global_position
		var icon_pos = map_center + Vector2(w_pos.x, w_pos.z) * map_scale
		player_icon.position = icon_pos

@onready var tip_timer = $HudRoot/Timer
func show_tip(msg: String):
	tip_label.text = msg
	await get_tree().create_timer(1.0).timeout
	tip_label.text=""
