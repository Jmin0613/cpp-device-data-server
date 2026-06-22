```mermaid
flowchart LR
    A[Client Connection] --> B[TcpServer]
    B --> C[accept]
    C --> D[ThreadPool Queue]
    D --> E[Worker Thread]
    E --> F[ClientSession]
    F --> G[recv loop]
    G --> H[newline 기준 packet 분리]
    H --> I[PacketProcessor]

    F -. connection 단위로 worker 점유 .-> E
 ```