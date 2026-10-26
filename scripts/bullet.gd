extends RigidBody3D

@export var speed: float = 5.0
@export var damage: float = 10.0

signal hit_tip(msg: String)

func _ready():
	linear_velocity = global_transform.basis.z * -speed

func _on_body_entered(body: Node3D):
	var tip_msg = " "
	if body.name == "ArmorFront":
		tip_msg = "击中装甲！"
		body.take_damage(damage)
	else:
		tip_msg = "子弹碰到了： %s" % body.name
	emit_signal("hit_tip", tip_msg)
	queue_free()
