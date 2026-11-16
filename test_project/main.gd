extends Node2D

func _ready() -> void:
	test()

func test() -> void:
	var my_class : Test = Test.new()
	
	my_class.say_hello()
	
	print(my_class.my_data)
