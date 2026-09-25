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
            searchInput.placeholder = '🔍 Buscar medicamento pelo nome...';
            searchInput.style.cssText = 'width: 100%; padding: 10px; margin-bottom: 15px; border: 1px solid #ccc; border-radius: 4px; box-sizing: border-box; font-size: 1rem;';
            searchInput.addEventListener('input', () => renderTable());
            
            const h2Table = tableContainer.querySelector('h2');
            if (h2Table) {
                h2Table.insertAdjacentElement('afterend', searchInput);
            }
        }
    }

    function renderDashboard() {
        const hoje = new Date().toISOString().split('T')[0];
        const total = medications.length;
        const vencidos = medications.filter(med => med.expiry < hoje).length;
        const estoqueBaixo = medications.filter(med => Number(med.quantity) <= 2 && med.expiry >= hoje).length;
        const regulares = total - vencidos - estoqueBaixo;

        const statsDiv = document.getElementById('dashboard-stats');
        if (statsDiv) {
            statsDiv.innerHTML = `
                <div style="background: #e3f2fd; padding: 12px; border-radius: 6px; text-align: center; border-left: 4px solid #2196f3;">
                    <span style="font-size: 0.85rem; color: #555;">Total</span><br><strong>${total}</strong>
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

        const medicamentosFiltrados = medications.filter(med => med.name.toLowerCase().includes(termoBusca));

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
                <td>${med.quantity}</td>
                <td>${med.expiry.split('-').reverse().join('/')} ${statusHtml}</td>
                <td>${med.fileira || '-'}</td>
                <td>${med.coluna || '-'}</td>
                <td>
                    <button type="button" onclick="editMedication(${realIndex})" style="background-color: #f39c12; color: white; border: none; padding: 5px 8px; border-radius: 4px; cursor: pointer; font-size: 0.85rem; margin-right: 4px;">Editar</button>
                    <button type="button" onclick="removeMedication(${realIndex})" style="background-color: #c0392b; color: white; border: none; padding: 5px 8px; border-radius: 4px; cursor: pointer; font-size: 0.85rem;">Remover</button>
                </td>
            `;
            medicationList.appendChild(tr);
        });
    }

    function checkAlerts() {
        if (!alertaVencidos || !textoAlerta) return;
        const hoje = new Date().toISOString().split('T')[0];
        const vencidos = medications.filter(med => med.expiry < hoje);
        const estoqueBaixo = medications.filter(med => Number(med.quantity) <= 2);

        let mensagens = [];

        if (vencidos.length > 0) {
            mensagens.push(`${vencidos.length} produto(s) vencido(s)`);
        }
        if (estoqueBaixo.length > 0) {
            mensagens.push(`${estoqueBaixo.length} produto(s) com estoque baixo`);
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
            const quantityInput = document.getElementById('med-quantity');
            const expiryInput = document.getElementById('med-expiry');
            const fileiraInput = document.getElementById('med-fileira');
            const colunaInput = document.getElementById('med-coluna');

            if (!nameInput || !quantityInput || !expiryInput || !fileiraInput || !colunaInput) return;

            const name = nameInput.value;
            const quantity = quantityInput.value;
            const expiry = expiryInput.value;
            const fileira = fileiraInput.value;
            const coluna = colunaInput.value;

            if (editIndex !== null) {
                medications[editIndex] = { name, quantity, expiry, fileira, coluna };
                editIndex = null;
                const submitBtn = form.querySelector('button[type="submit"]');
                if (submitBtn) submitBtn.textContent = 'Salvar Medicamento';
            } else {
                medications.push({ name, quantity, expiry, fileira, coluna });

                const hoje = new Date().toISOString().split('T')[0];
                if (expiry < hoje) {
                    enviarAlerta(name, quantity, expiry, fileira, coluna, "VENCIDO");
                } 
                if (Number(quantity) <= 2) {
                    enviarAlerta(name, quantity, expiry, fileira, coluna, "ESTOQUE_BAIXO");
                }
            }

            saveAndRender();
            form.reset();
        });
    }

    window.removeMedication = function(index) {
        medications.splice(index, 1);
        saveAndRender();
    };

    window.editMedication = function(index) {
        const med = medications[index];
        const nameInput = document.getElementById('med-name');
        const quantityInput = document.getElementById('med-quantity');
        const expiryInput = document.getElementById('med-expiry');
        const fileiraInput = document.getElementById('med-fileira');
        const colunaInput = document.getElementById('med-coluna');

        if (nameInput) nameInput.value = med.name;
        if (quantityInput) quantityInput.value = med.quantity;
        if (expiryInput) expiryInput.value = med.expiry;
        if (fileiraInput) fileiraInput.value = med.fileira || '';
        if (colunaInput) colunaInput.value = med.coluna || '';
        
        editIndex = index;
        const submitBtn = form.querySelector('button[type="submit"]');
        if (submitBtn) submitBtn.textContent = 'Salvar Alteração';
        
        window.scrollTo({ top: 0, behavior: 'smooth' });
    };

    criarElementosVisuais();
    saveAndRender();
});

function enviarAlerta(produtoNome, quantidadeProduto, dataValidade, fileiraProduto, colunaProduto, tipoAlerta) {
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
        coluna: colunaProduto,
        email: "safestock.projete2026@gmail.com"
    };

    emailjs.send('service_z8bwtch', templateIdUsado, templateParams)
        .then((response) => {
           console.log('E-mail enviado com sucesso!', response.status, response.text);
        }, (error) => {
           console.error('Erro ao enviar o e-mail:', error);
        });
}