
../deps/bin/protoc --cpp_out=. Framework/graph.proto

# 
#sed -i "s/google/weibo/g" `grep google -rl Framework/graph.pb.*`

../deps/bin/protoc --cpp_out=. Interfaces/rank_interface.proto
#sed -i "s/google/weibo/g" `grep google -rl Interfaces/rank_interface.pb.*`
../deps/bin/protoc --java_out=./JavaTools Interfaces/rank_interface.proto

#process/proto/protoc_7 --cpp_out=. Framework/hebe.proto
#sed -i "s/google/weibo/g" `grep google -rl Framework/hebe.pb.*`

#cd -

