class_name MotorController extends MarginContainer

@onready var lbl_angle: Label = $vbox1/hboxAngle/lbl11
@onready var lbl_min: Label = $vbox1/hboxLimit/lblMin
@onready var lbl_max: Label = $vbox1/hboxLimit/lblMax
@onready var lbl_title: Label = $vbox1/lblTitle
@onready var bt_minus: Button = $vbox1/hboxAngle/btMinus
@onready var bt_plus: Button = $vbox1/hboxAngle/btPlus
@onready var bt_online: CheckButton = $vbox1/hboxAngle/btOnline
@onready var slider_spd: HSlider = $vbox1/hboxSpd/sliderSpd
@onready var lbl_spd: Label = $vbox1/hboxSpd/lblSpd

var _title: String = ""
@export var Title: String:
	get: return _title
	set(v): 
		_title = v
		if is_node_ready(): lbl_title.text = _title

var IsOnline: bool:
	get: return bt_online.button_pressed

var _mid: int = 0
@export var MotorId: int:
	get: return _mid
	set(v): if (v < 4 and v >= 0): _mid = v

var _angle0: int = 90
var angle: int = 90
var angle_min: int = 0
var angle_max: int = 180

var _spd0: int = 1

var _btMinusHold: float = 0
var _btMinusTotalHold: float = 0
var _btPlusHold: float = 0
var _btPlusTotalHold: float = 0

func _ready() -> void:
	lbl_title.text = _title

func _process(delta: float) -> void:
	if is_node_ready():
		var a: int = 0
		
		if bt_minus.button_pressed:
			_btMinusHold += delta
			_btMinusTotalHold += delta
			a = 1 if _btMinusTotalHold < 1 else 5
			if _btMinusHold > 0.1:
				a = max(angle - a, angle_min)
				if a != angle:
					angle = a
					lbl_angle.text = str(angle)
				_btMinusHold -= 0.1

		elif bt_plus.button_pressed:
			_btPlusHold += delta
			_btPlusTotalHold += delta
			a = 1 if _btPlusTotalHold < 1 else 5
			if _btPlusHold > 0.1:
				a = min(angle + a, angle_max)
				if a != angle:
					angle = a
					lbl_angle.text = str(angle)
				_btPlusHold -= 0.1

func _on_btminus_button_up() -> void:
	_btMinusTotalHold = 0
	_btMinusHold = 0

func _on_btminus_button_down() -> void:
	_btMinusHold = 0.1
	_btMinusTotalHold = 0.1

func _on_btplus_button_up() -> void:
	_btPlusHold = 0
	_btPlusTotalHold = 0

func _on_btplus_button_down() -> void:
	_btPlusHold = 0.1
	_btPlusTotalHold = 0.1

func get_msg() -> String:
	var s: String = ""
	if angle != _angle0:
		s = ":1%02x%02x;" % [(4 + _mid), angle]
		_angle0 = angle
	elif slider_spd.value != _spd0:
		var spd: int = slider_spd.value
		s = ":1%02x%02x;" % [(8 + _mid), spd]
		_spd0 = spd
	return s

func _on_sliderspd_drag_ended(value_changed: bool) -> void:
	if value_changed:
		lbl_spd.text = str(slider_spd.value)

func update_mem(addr, data):
	#print("Update mem %d: %d = %d" % [_mid, addr, data])
	match addr - _mid:
		4:
			#_angle0 = data
			angle = data
			lbl_angle.text = str(data)
		8:
			_spd0 = data
			slider_spd.value = data
		16:
			lbl_min.text = str(data)
		20:
			lbl_max.text = str(data)
