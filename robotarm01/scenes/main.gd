extends Node2D

@onready var bt_connect: Button = $UILayer/Control/vboxTitle/hbox/btConnect
@onready var pnl_connect: MarginContainer = $UILayer/Control/cSrv

@onready var motor_ctrl1: MotorController = $UILayer/Control/vboxMotors/MotorCtrl
@onready var motor_ctrl2: MotorController = $UILayer/Control/vboxMotors/MotorCtrl2
@onready var motor_ctrl3: MotorController = $UILayer/Control/vboxMotors/MotorCtrl3
@onready var motor_ctrl4: MotorController = $UILayer/Control/vboxMotors/MotorCtrl4

@onready var bt_send_multi: Button = $UILayer/Control/vboxCmds/btSendMulti
@onready var history_list: MarginContainer = $UILayer/Control/HistoryList

const Client = preload("res://scripts/client.gd")
var _client: Client = Client.new()
var _wait_response: float = 0
var _commands: Array[String] = []

func _ready() -> void:
	_client.sgn_connected.connect(_handle_client_connected)
	_client.sgn_disconnected.connect(_handle_client_disconnected)
	_client.sgn_data.connect(_handle_client_data)
	
	add_child(_client)

func _process(delta: float) -> void:
	if _wait_response > 0.01: 
		_wait_response -= delta
		if _wait_response <= 0.01:
			print("No response!")
			__send_cmd()


func _handle_client_connected() -> void:
	set_btConnect_text("Connected", Color(0, 255, 0))
	bt_send_multi.disabled = false
	#__send_cmd(":50408;")
	__send_cmd(":50410;")

func _on_csrv_connect(addr: String, port: int) -> void:
	pnl_connect.visible = false
	_client.connect_to_host(addr, port)
	set_btConnect_text("Connecting", Color(255, 255, 0))

func _on_btconnect_button_up() -> void:
	if _client.Status == StreamPeerTCP.STATUS_NONE:
		if pnl_connect.visible: pnl_connect.visible = false
		else: pnl_connect.visible = true
	else:
		_client.disconnect_from_host()

func _handle_client_disconnected() -> void:
	set_btConnect_text("Disconnected", Color(255, 255, 255))
	bt_send_multi.disabled = true

func _handle_client_error() -> void:
	set_btConnect_text("Error", Color(255, 0, 0))

func _handle_client_data(data) -> void:
	history_list.add_msg(":%s;" % data.get_string_from_ascii(), 2)
	_wait_response = 0
	var l = len(data)
	if l > 4:
		if data[0] == 52:	# '4'
			var addr = __hex2byte(data, 1, 2)
			var val = __hex2byte(data, 3, 2)
			__update_motor(addr, val)
		if data[0] == 53:	# '5'
			var addr0 = __hex2byte(data, 1, 2)
			var n = __hex2byte(data, 3, 2)
			for i in range(n):
				if 6 + i >= l: break
				var val = __hex2byte(data, 5 + i * 2, 2)
				__update_motor(addr0 + i, val)
	__send_cmd()

func set_btConnect_text(s: String, c: Color):
	bt_connect.text = s
	bt_connect.set("theme_override_colors/font_hover_pressed_color", c)
	bt_connect.set("theme_override_colors/font_hover_color", c)
	bt_connect.set("theme_override_colors/font_pressed_color", c)
	bt_connect.set("theme_override_colors/font_color", c)

func _on_btsendmulti_button_up() -> void:
	if _client.Status != StreamPeerTCP.STATUS_CONNECTED: return
	var s = ""
	while true:
		s = motor_ctrl1.get_msg()
		if s != "": _commands.append(s)
		else: break;
	while true:
		s = motor_ctrl2.get_msg()
		if s != "": _commands.append(s)
		else: break;
	while true:
		s = motor_ctrl3.get_msg()
		if s != "": _commands.append(s)
		else: break;
	while true:
		s = motor_ctrl4.get_msg()
		if s != "": _commands.append(s)
		else: break;
	__send_cmd()

# Send command in _commands if connect to robot
func __send_cmd(cmd: String = ""):
	if _client.Status != StreamPeerTCP.STATUS_CONNECTED or _wait_response > 0.1: return
	var s: String = cmd
	if s == "" and len(_commands) > 0:
		s = _commands.pop_at(0)
	if s != "":
		history_list.add_msg(s, 0)
		_client.send(s.to_utf8_buffer())
		_wait_response = 3

func __update_motor(addr: int, val: int):
	match addr % 4:
		0: motor_ctrl1.update_mem(addr, val)
		1: motor_ctrl2.update_mem(addr, val)
		2: motor_ctrl3.update_mem(addr, val)
		3: motor_ctrl4.update_mem(addr, val)

func __hex2byte(data: PackedByteArray, i0: int, l: int) -> int:
	var x: int = 0
	var i: int = 0
	var n: int = len(data)
	while i < l and i + i0 < n:
		var c = data[i0 + i]
		if c >= 48 and c < 58:
			x *= 16
			x += c - 48
		elif c >= 97 and c < 103:
			x *= 16
			x += c - 87	# 'a' + 10
		elif c >= 65 and c < 71:
			x *= 16
			x += c - 55		# 'A' + 10
		else: break
		i += 1
	return x
