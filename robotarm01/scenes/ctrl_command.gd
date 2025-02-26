extends HBoxContainer

@onready var lbl_time: Label = $lblTime
@onready var lbl_msg: Label = $lblMsg

func _ready() -> void:
	pass # Replace with function body.

func set_msg(s: String, kind: int):
	var t = Time.get_datetime_dict_from_system()
	lbl_time.text = "%02d:%02d:%02d" % [t["hour"], t["minute"], t["second"]]
	lbl_msg.text = s
	if kind == 2:
		lbl_msg.self_modulate = Color(0, 223, 0)
		#lbl_msg.horizontal_alignment = HORIZONTAL_ALIGNMENT_RIGHT
