file(GLOB MAIN_SRCS "*.cpp" "*.c")
file(GLOB VIEW_SRCS "views/*.cpp")

idf_component_register(
    SRCS ${MAIN_SRCS} ${VIEW_SRCS}
    PRIV_REQUIRES spi_flash LovyanGFX
    INCLUDE_DIRS . views
    REQUIRES esp_adc
)
