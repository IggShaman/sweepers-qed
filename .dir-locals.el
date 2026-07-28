; elisp
;; ((nil . ((compile-command . "make -C build -k -j$(nproc)"))))
((nil . ((compile-command . "cmake --build build -j$(nproc) && ctest --test-dir build --output-on-failure"))))

