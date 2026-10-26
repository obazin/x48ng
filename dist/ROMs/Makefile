.PHONY: get-roms mrproper clean

ROMS = sxrom-a sxrom-b sxrom-c sxrom-d sxrom-e sxrom-j gxrom-l gxrom-m gxrom-p gxrom-r

get-roms: $(ROMS)

mrproper:
	rm -f $(addprefix ./,$(ROMS)) $(addsuffix .zip,$(ROMS)) $(addsuffix .part,$(ROMS))

clean: mrproper

# Through temporary files: a failed download must not leave a truncated ROM that make would then consider up to date
$(ROMS):
	curl --fail --location --silent --show-error --output "$@.zip" "https://www.hpcalc.org/hp48/pc/emulators/$@.zip" \
		&& unzip -p "$@.zip" > "$@.part" && mv "$@.part" "$@"; \
		status=$$?; rm -f "$@.zip" "$@.part"; exit $$status
