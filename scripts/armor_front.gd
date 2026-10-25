extends Area3D

var hp = 100

func _on_armor_front_body_entered(body: Node3D):
	print("=== 进入函数了！碰到物体： ",body.name)
	if body.name == "Bullet":
		hp -= 10
		print("装甲被击中！剩余血量： ",hp)
		body.queue_free()
		if hp <= 0:
			print("装甲击毁！")
