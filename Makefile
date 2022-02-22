.PHONY: clean All

All:
	@echo "----------Building project:[ Labb5 - Debug ]----------"
	@"$(MAKE)" -f  "Labb5.mk" && "$(MAKE)" -f  "Labb5.mk" PostBuild
clean:
	@echo "----------Cleaning project:[ Labb5 - Debug ]----------"
	@"$(MAKE)" -f  "Labb5.mk" clean
