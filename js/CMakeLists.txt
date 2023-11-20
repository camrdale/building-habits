

set(js-compiled "${CMAKE_CURRENT_LIST_DIR}/index.js")
set(js-bundled "${CMAKE_CURRENT_LIST_DIR}/building-habits.bundled.js")
set(js-static "${CMAKE_CURRENT_LIST_DIR}/../static/building-habits.bundled.js")

add_custom_command(
    OUTPUT ${js-compiled}
    DEPENDS src/index.ts
    WORKING_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}"
    COMMAND npm run build
)

add_custom_target(BUILD-JS DEPENDS ${js-compiled})

add_custom_command(
    OUTPUT ${js-bundled}
    DEPENDS ${js-compiled}
    WORKING_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}"
    COMMAND npm run bundle
)

add_custom_target(BUNDLE-JS ALL DEPENDS ${js-bundled})

add_custom_command(
  TARGET BUNDLE-JS POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different ${js-bundled} ${js-static}
)
