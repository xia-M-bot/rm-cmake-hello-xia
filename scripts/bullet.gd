extends RigidBody3D

@export var speed: float = 5.0
@export var damage: float = 10.0

func _ready():
	linear_velocity = global_transform.basis.z * -speed

func _on_body_entered(body: Node3D):
	print("子弹碰到了： ",body.name)
	if body.name == "ArmorFront":
		print("击中装甲！")
		body.take_damage(damage)
	queue_free()
