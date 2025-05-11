mkdir GPTfs
python3 GPT_FUSE.py GPTfs > log.txt 2>&1 &

cd GPTfs

make_session() {
    echo "session: $1 input: $2"
    mkdir -p $1
    echo "$2" > $1/input
    echo "output: "
    cat $1/output
    echo -e "\n\nerror: "
    cat $1/error
    echo -e "\n\n"
}


make_session "session1" "Hello, world"
make_session "session2" "This is a test."
make_session "session3" "GPT-4 is amazing"
make_session "session4" "I love programming."
make_session "session5" "/no_think 来一个冷笑话1"

