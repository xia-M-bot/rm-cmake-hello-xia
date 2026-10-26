extends Area3D

func _ready():
	body_entered.connect(_on_body_entered)

func _on_body_entered(body):
	if body.has_method("reset"):
		body.hp = 100
		body.ammo = 40
		body.heat = 0
		body.is_overheated = false
		body.hud.show_tip("补给完成！")
