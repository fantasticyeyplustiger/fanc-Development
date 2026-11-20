extends Node2D

func _ready() -> void:
	test()

func test() -> void:
	var lexer := FanC.new()
	
	var source_code := "int x = 10;"
	
	print("Lexer from C++ (GDExtension):\nSource code:\n", source_code)
	var start_time := Time.get_ticks_usec()
	lexer.run(source_code)
	var end_time := Time.get_ticks_usec()
	print("Token Parse Time: %d" % (end_time - start_time))
