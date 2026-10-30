Requisitos Funcionais
- RF01 – Realizar login: O sistema deve permitir que usuários autenticados acessem a plataforma por meio de e-mail e senha.
- RF02 – Visualizar dashboard: O sistema deve apresentar um painel com informações atualizadas sobre os reservatórios monitorados.
- RF03 – Receber medições: O sistema deve receber os dados enviados pelos dispositivos ESP32 por meio do protocolo MQTT.
- RF04 – Calcular nível de óleo: O sistema deve calcular o nível estimado de óleo com base nas distâncias medidas pelo sensor e nas dimensões do reservatório.
- RF05 – Apresentar status do monitoramento: O sistema deve exibir o estado de funcionamento dos dispositivos e dos reservatórios monitorados.
- RF06 – Gerar relatórios: O sistema deve permitir a geração de relatórios contendo as medições registradas.
- RF07 – Visualizar alertas: O sistema deve apresentar alertas relacionados a níveis críticos, falhas de comunicação e possíveis anomalias.
- RF08 – Gerenciar reservatórios: O administrador deve poder cadastrar, consultar, editar e excluir informações dos reservatórios.
- RF09 – Armazenar histórico de medições: O sistema deve armazenar as medições recebidas para consultas e análises futuras.
- RF10 – Analisar dados: O sistema deve analisar os dados coletados para identificar possíveis anomalias no comportamento dos níveis de óleo.
- RF11 – Registrar anomalias: O sistema deve registrar os resultados das análises, incluindo a classificação das possíveis anomalias identificadas.
- RF12 – Identificar falhas de comunicação: O sistema deve identificar dispositivos que deixarem de transmitir medições dentro do intervalo esperado.
- RF13 – Associar medições: O sistema deve relacionar cada medição ao respectivo reservatório e dispositivo.
- RF14 – Registrar data e hora: O sistema deve registrar a data e o horário de cada medição recebida.

Requisitos Não Funcionais
- RNF01 – Usabilidade: O sistema deve possuir uma interface intuitiva e de fácil utilização, permitindo que os usuários acessem as informações sem dificuldades.
- RNF02 – Desempenho: O sistema deve apresentar as informações no dashboard de acordo com o recebimento das novas medições, evitando atrasos desnecessários.
- RNF03 – Segurança: O sistema deve possuir autenticação de usuários e controle de acesso conforme os níveis de permissão.
- RNF04 – Comunicação segura: A comunicação entre os dispositivos e a plataforma deve utilizar mecanismos que protejam os dados transmitidos.
- RNF05 – Armazenamento: O sistema deve armazenar as medições e informações dos reservatórios de forma organizada e persistente.
- RNF06 – Confiabilidade: O sistema deve identificar dados inválidos, inconsistências nas medições e falhas de comunicação.
- RNF07 – Escalabilidade: A arquitetura deve permitir a inclusão de novos reservatórios e dispositivos sem exigir grandes alterações no sistema.
- RNF08 – Integridade dos dados: O sistema deve garantir que as medições estejam corretamente associadas aos seus respectivos reservatórios e dispositivos.
- RNF09 – Apresentação visual: O dashboard deve apresentar informações de maneira clara, utilizando indicadores, gráficos e alertas para facilitar a interpretação dos dados.
- RNF10 – Manutenibilidade: O sistema deve possuir uma estrutura que facilite futuras atualizações, correções e melhorias.
