emailjs.init("IkgFer8WsM9rmiefY");

document.addEventListener('DOMContentLoaded', () => {
    const form = document.getElementById('medication-form');
    const medicationList = document.getElementById('medication-list');
    const alertaVencidos = document.getElementById('alerta-vencidos');
    const textoAlerta = document.getElementById('texto-alerta');

    let medications = JSON.parse(localStorage.getItem('medications')) || [];
    let editIndex = null; // Controla se estamos editando um item

    function saveAndRender() {
        localStorage.setItem('medications', JSON.stringify(medications));
        renderDashboard();
        renderTable();
        checkAlerts();
    }

    // Cria o painel de estatísticas e a barra de busca dinamicamente se não existirem
    function criarElementosVisuais() {
        const tableContainer = document.querySelector('#conteudo-estoque .table-container');

        if (!document.getElementById('dashboard-stats') && form) {
            const statsDiv = document.createElement('div');
            statsDiv.id = 'dashboard-stats';
            statsDiv.style.cssText = 'display: grid; grid-template-columns: repeat(auto-fit, minmax(110px, 1fr)); gap: 10px; margin-bottom: 20px;';
            form.parentNode.insertBefore(statsDiv, form);
        }

        if (!document.getElementById('search-med') && tableContainer) {
            const searchInput = document.createElement('input');
            searchInput.type = 'text';
            searchInput.id = 'search-med';
            searchInput.placeholder = '🔍 Buscar medicamento pelo nome ou ID...';
            searchInput.style.cssText = 'width: 100%; padding: 10px; margin-bottom: 15px; border: 1px solid #ccc; border-radius: 4px; box-sizing: border-box; font-size: 1rem;';
            searchInput.addEventListener('input', () => renderTable());
            
            const h2Table = tableContainer.querySelector('h2');
            if (h2Table) {
                h2Table.insertAdjacentElement('afterend', searchInput);
            }
        }

        // Atualiza o cabeçalho da tabela para substituir COLUNA por ID
        const tabelaHtml = tableContainer ? tableContainer.querySelector('table') : null;
        if (tabelaHtml) {
            const headerRow = tabelaHtml.querySelector('thead tr');
            if (headerRow) {
                headerRow.innerHTML = `
                    <th>NOME</th>
                    <th>ID</th>
                    <th>QUANTIDADE</th>
                    <th>VALIDADE / STATUS</th>
                    <th>FILEIRA</th>
                    <th>AÇÕES</th>
                `;
            }
        }
    }

    function renderDashboard() {
        const hoje = new Date().toISOString().split('T')[0];
        
        const total = medications.reduce((acc, med) => acc + (med && med.quantity ? Number(med.quantity) : 0), 0);
        
        const vencidos = medications
            .filter(med => med && med.expiry && med.expiry < hoje)
            .reduce((acc, med) => acc + Number(med.quantity), 0);

        const estoqueBaixo = medications
            .filter(med => med && Number(med.quantity) <= 2 && med.expiry >= hoje)
            .reduce((acc, med) => acc + Number(med.quantity), 0);

        const regulares = medications
            .filter(med => med && med.expiry >= hoje && Number(med.quantity) > 2)
            .reduce((acc, med) => acc + Number(med.quantity), 0);

        const statsDiv = document.getElementById('dashboard-stats');
        if (statsDiv) {
            statsDiv.innerHTML = `
                <div style="background: #e3f2fd; padding: 12px; border-radius: 6px; text-align: center; border-left: 4px solid #2196f3;">
                    <span style="font-size: 0.85rem; color: #555;">Qtd. Total Estoque</span><br><strong>${total}</strong>
                </div>
                <div style="background: #e8f5e9; padding: 12px; border-radius: 6px; text-align: center; border-left: 4px solid #4caf50;">
                    <span style="font-size: 0.85rem; color: #555;">Regulares</span><br><strong>${regulares}</strong>
                </div>
                <div style="background: #fff3cd; padding: 12px; border-radius: 6px; text-align: center; border-left: 4px solid #ffc107;">
                    <span style="font-size: 0.85rem; color: #555;">Estoque Baixo</span><br><strong>${estoqueBaixo}</strong>
                </div>
                <div style="background: #ffebee; padding: 12px; border-radius: 6px; text-align: center; border-left: 4px solid #f44336;">
                    <span style="font-size: 0.85rem; color: #555;">Vencidos</span><br><strong>${vencidos}</strong>
                </div>
            `;
        }
    }

    function renderTable() {
        if (!medicationList) return;
        medicationList.innerHTML = '';
        const hoje = new Date().toISOString().split('T')[0];
        const termoBusca = document.getElementById('search-med') ? document.getElementById('search-med').value.toLowerCase() : '';

        const medicamentosFiltrados = medications.filter(med => {
            if (!med || !med.name) return false;
            return med.name.toLowerCase().includes(termoBusca) || (med.id && med.id.toLowerCase().includes(termoBusca));
        });

        medicamentosFiltrados.forEach((med) => {
            const realIndex = medications.indexOf(med);
            const tr = document.createElement('tr');
            
            if (med.expiry < hoje) {
                tr.style.backgroundColor = '#ffd2d2';
            } else if (Number(med.quantity) <= 2) {
                tr.style.backgroundColor = '#fff3cd';
            }

            let statusHtml = '';
            if (med.expiry < hoje) {
                statusHtml += '<strong>(VENCIDO!)</strong> ';
            }
            if (Number(med.quantity) <= 2) {
                statusHtml += '<strong>(ESTOQUE BAIXO!)</strong>';
            }

            tr.innerHTML = `
                <td>${med.name}</td>
                <td>${med.id || '-'}</td>
                <td>${med.quantity}</td>
                <td>${med.expiry.split('-').reverse().join('/')} ${statusHtml}</td>
                <td>${med.fileira || '-'}</td>
                <td>
                    <button type="button" onclick="baixarMedication(${realIndex})" style="background-color: #27ae60; color: white; border: none; padding: 5px 8px; border-radius: 4px; cursor: pointer; font-size: 0.85rem; margin-right: 4px;">Dar Baixa</button>
                    <button type="button" onclick="editMedication(${realIndex})" style="background-color: #f39c12; color: white; border: none; padding: 5px 8px; border-radius: 4px; cursor: pointer; font-size: 0.85rem;">Editar</button>
                </td>
            `;
            medicationList.appendChild(tr);
        });
    }

    function checkAlerts() {
        if (!alertaVencidos || !textoAlerta) return;
        const hoje = new Date().toISOString().split('T')[0];
        const vencidos = medications.filter(med => med && med.expiry && med.expiry < hoje);
        const estoqueBaixo = medications.filter(med => med && Number(med.quantity) <= 2);

        let mensagens = [];

        if (vencidos.length > 0) {
            mensagens.push(`${vencidos.length} lote(s) vencido(s)`);
        }
        if (estoqueBaixo.length > 0) {
            mensagens.push(`${estoqueBaixo.length} lote(s) com estoque baixo`);
        }

        if (mensagens.length > 0) {
            alertaVencidos.style.display = 'block';
            alertaVencidos.style.borderTop = '4px solid #e74c3c';
            textoAlerta.textContent = `Atenção! Alerta: ${mensagens.join(' e ')}.`;
        } else {
            alertaVencidos.style.display = 'none';
        }
    }

    if (form) {
        form.addEventListener('submit', (e) => {
            e.preventDefault();

            const nameInput = document.getElementById('med-name');
            const idInput = document.getElementById('med-id');
            const quantityInput = document.getElementById('med-quantity');
            const expiryInput = document.getElementById('med-expiry');
            const fileiraInput = document.getElementById('med-fileira');

            if (!nameInput || !quantityInput || !expiryInput || !fileiraInput) return;

            const name = nameInput.value;
            const id = idInput ? idInput.value : '';
            const quantity = Number(quantityInput.value);
            const expiry = expiryInput.value;
            const fileira = fileiraInput.value;

            if (editIndex !== null) {
                medications[editIndex] = { name, id, quantity, expiry, fileira };
                editIndex = null;
                const submitBtn = form.querySelector('button[type="submit"]');
                if (submitBtn) submitBtn.textContent = 'Salvar Medicamento';
            } else {
                medications.push({ name, id, quantity, expiry, fileira });

                const hoje = new Date().toISOString().split('T')[0];
                if (expiry < hoje) {
                    enviarAlerta(name, quantity, expiry, fileira, id, "VENCIDO");
                } 
                if (Number(quantity) <= 2) {
                    enviarAlerta(name, quantity, expiry, fileira, id, "ESTOQUE_BAIXO");
                }
            }

            saveAndRender();
            form.reset();
        });
    }

    window.baixarMedication = function(index) {
        const med = medications[index];
        const qtdParaRetirar = prompt(`Quantas unidades de "${med.name}" deseja retirar? (Atual: ${med.quantity})`);
        
        if (qtdParaRetirar === null || qtdParaRetirar.trim() === "") return;

        const qtdNum = Number(qtdParaRetirar);

        if (isNaN(qtdNum) || qtdNum <= 0) {
            alert("Por favor, digite um número válido!");
            return;
        }

        if (qtdNum > Number(med.quantity)) {
            alert("A quantidade a retirar é maior do que o estoque atual deste lote!");
            return;
        }

        med.quantity = Number(med.quantity) - qtdNum;

        if (med.quantity <= 0) {
            medications.splice(index, 1);
        }

        saveAndRender();
    };

    window.editMedication = function(index) {
        const med = medications[index];
        const nameInput = document.getElementById('med-name');
        const idInput = document.getElementById('med-id');
        const quantityInput = document.getElementById('med-quantity');
        const expiryInput = document.getElementById('med-expiry');
        const fileiraInput = document.getElementById('med-fileira');

        if (nameInput) nameInput.value = med.name;
        if (idInput) idInput.value = med.id || '';
        if (quantityInput) quantityInput.value = med.quantity;
        if (expiryInput) expiryInput.value = med.expiry;
        if (fileiraInput) fileiraInput.value = med.fileira || '';
        
        editIndex = index;
        const submitBtn = form.querySelector('button[type="submit"]');
        if (submitBtn) submitBtn.textContent = 'Salvar Alteração';
        
        window.scrollTo({ top: 0, behavior: 'smooth' });
    };

    criarElementosVisuais();
    saveAndRender();
});

function enviarAlerta(produtoNome, quantidadeProduto, dataValidade, fileiraProduto, idProduto, tipoAlerta) {
    let templateIdUsado = '';

    if (tipoAlerta === "VENCIDO") {
        templateIdUsado = 'template_7wzu6yh';
    } else {
        templateIdUsado = 'template_u9emkft';
    }

    const templateParams = {
        product_name: produtoNome,
        quantity: quantidadeProduto,
        expiry_date: dataValidade,
        fileira: fileiraProduto,
        id_produto: idProduto,
        email: "safestock.projete2026@gmail.com"
    };

    emailjs.send('service_z8bwtch', templateIdUsado, templateParams)
        .then((response) => {
           console.log('E-mail enviado com sucesso!', response.status, response.text);
        }, (error) => {
           console.error('Erro ao enviar o e-mail:', error);
        });
}