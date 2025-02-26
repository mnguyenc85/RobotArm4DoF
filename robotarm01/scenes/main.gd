extends Node2D

@onready var csrv: MarginContainer = $UILayer/vbox/Control/cSrv
@onready var bt_connect: Button = $UILayer/vbox/Control/vboxTitle/hbox/btConnect

@onready var motor_ctrl1: MotorController = $UILayer/vbox/Control/vboxMotors/MotorCtrl
@onready var motor_ctrl2: MotorController = $UILayer/vbox/Control/vboxMotors/MotorCtrl2
@onready var motor_ctrl3: MotorController = $UILayer/vbox/Control/vboxMotors/MotorCtrl3
@onready var motor_ctrl4: MotorController = $UILayer/vbox/Control/vboxMotors/MotorCtrl4

@onready var bt_send_multi: Button = $UILayer/vbox/Control/vboxCmds/btSendMulti

const Client = preload("res://scripts/client.gd")
var _client: Client = Client.new()
var _wait_response: float = 0

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

func _handle_client_connected() -> void:
	bt_connect.text = "Connected"
	change_btConnect_color(Color(0, 255, 0))
	bt_send_multi.disabled = false
	__send_cmd(":50408;")

func _on_csrv_connect(addr: String, port: int) -> void:
	csrv.visible = false
	_client.connect_to_host(addr, port)

func _on_btconnect_button_up() -> void:
	if _client.Status == StreamPeerTCP.STATUS_NONE:
		if csrv.visible: csrv.visible = false
		else: csrv.visible = true
	else:
		_client.disconnect_from_host()

func _handle_client_disconnected() -> void:
	bt_connect.text = "Disconnected"
	change_btConnect_color(Color(255, 255, 255))
	bt_send_multi.disabled = true

func _handle_client_error() -> void:
	bt_connect.text = "Error"
	change_btConnect_color(Color(255, 0, 0))

func _handle_client_data(data) -> void:
	print(data.get_string_from_ascii())
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

func change_btConnect_color(c: Color):
	bt_connect.set("theme_override_colors/font_hover_pressed_color", c)
	bt_connect.set("theme_override_colors/font_hover_color", c)
	bt_connect.set("theme_override_colors/font_pressed_color", c)
	bt_connect.set("theme_override_colors/font_color", c)

func _on_btsendmulti_button_up() -> void:
	var s = motor_ctrl1.get_msg()
	if s == "": s = motor_ctrl2.get_msg()
	if s == "": s = motor_ctrl3.get_msg()
	if s == "": s = motor_ctrl4.get_msg()
	if s != "":
		if _client.Status == StreamPeerTCP.STATUS_CONNECTED:
			__send_cmd(s)

func __send_cmd(s: String):
	if _wait_response > 0.01:
		print("Waiting for response, can't send new command.")
		return
	print("Send: %s" % s)
	var data = s.to_utf8_buffer()
	_client.send(data)
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
