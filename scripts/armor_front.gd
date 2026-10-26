extends StaticBody3D

signal armor_hp_changed(hp: int)

var hp: int = 100

func _on_armor_front_body_entered(body: Node3D):
	print("=== 进入函数了！碰到物体： ",body.name)
	if body.name == "Bullet":
		hp -= 10
		print("装甲被击中！剩余血量： ",hp)
		body.queue_free()
		if hp <= 0:
			hp = 0
			print("装甲击毁！")

func take_damage(dmg: int):
	hp -= dmg
	if hp < 0:
		hp = 0
	print("装甲扣血，剩余HP:",hp)
	armor_hp_changed.emit(hp)
