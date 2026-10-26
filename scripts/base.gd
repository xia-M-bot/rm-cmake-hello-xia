extends Node3D

signal base_hp_changed(hp: int)
signal base_destroyed

@export var max_hp: int = 500
var hp: int

func _ready():
	hp = max_hp
	print("基地初始化完成，hp=",hp)
	base_hp_changed.emit(hp)

func _on_body_entered(body):
	# 子弹打过来
	if body.is_in_group("bullets"):
		take_damage(10)
		body.queue_free()

func take_damage(dmg: int):
	hp -= dmg
	if hp < 0:
		hp = 0
	base_hp_changed.emit(hp)
	if hp == 0:
		base_destroyed.emit()
