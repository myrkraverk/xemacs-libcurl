
;;	     Copyright 2015 Johann 'Myrkraverk' Oskarsson
;;			johann@myrkraverk.com

;; License: same as libcurl.

(load-module "curl")

(curl-version)

(setq curl (curl-easy-init))

(curl-easy-setopt curl 'curlopt-url "http://curl.haxx.se/")

(curl-easy-setopt curl 'curlopt-writefunction (lambda (s a)
						(with-temp-buffer
						  (insert s)
						  (append-to-buffer "*curl*"
								 (point-min)
								 (point-max))
						(length s))))

(curl-easy-perform curl) ;; C-x C-e here fills the beffer below.


